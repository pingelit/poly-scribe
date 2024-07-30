%% Main function to generate tests
function tests = integrationTest
    tests = functiontests(localfunctions);
end
    
%% Test Functions
function testBaseCreation(testCase)
    % We can create a Base object
    d1 = Base;
    d1.str_vec = "foo";
    d1.str_vec = ["foo" "bar"];
%     d1.union_member = 2;
%     d1.union_member = int32(2);
    d1.vec_3d = [1 2 3];
end

function testDerivedOneCreation(testCase)
    % We can create a DerivedOne object
    d1 = DerivedOne;
    d1.str_vec = "foo";
    d1.str_vec = ["foo" "bar"];
%     d1.union_member = 2;
%     d1.union_member = int32(2);
    d1.vec_3d = [1 2 3];
    d1.string_map.bar = "baz";
end

function testDerivedTwoCreation(testCase)
    % We can create a DerivedOne object
    d1 = DerivedTwo;
    d1.str_vec = "foo";
    d1.str_vec = ["foo" "bar"];
%     d1.union_member = 2;
%     d1.union_member = int32(2);
    d1.vec_3d = [1 2 3];
    d1.optional_value= 3.131;
end

function testDerivedOneTypeCheck(testCase)
    % DerivedOne object is type checked
    function setField(field_name, value)
        d1 = DerivedOne;
        d1.(field_name) = value;
    end
    verifyError(testCase,@()setField('str_vec', struct()),'MATLAB:validation:UnableToConvert');
%     verifyError(testCase,@()setField('union_member', 'foo'),'MATLAB:validation:IncompatibleSize');
%     verifyError(testCase,@()setField('union_member', "foo"),'MATLAB:validators:mustBeA');
    verifyError(testCase,@()setField('vec_3d', [1,2]),'MATLAB:validation:IncompatibleSize');
    verifyError(testCase,@()setField('string_map', [1,2]),'MATLAB:validation:UnableToConvert');
end

function testDerivedTwoTypeCheck(testCase)
    % DerivedOne object is type checked
    function setField(field_name, value)
        d1 = DerivedTwo;
        d1.(field_name) = value;
    end
    verifyError(testCase,@()setField('str_vec', struct()),'MATLAB:validation:UnableToConvert');
%     verifyError(testCase,@()setField('union_member', 'foo'),'MATLAB:validation:IncompatibleSize');
%     verifyError(testCase,@()setField('union_member', "foo"),'MATLAB:validators:mustBeA');
    verifyError(testCase,@()setField('vec_3d', [1,2]),'MATLAB:validation:IncompatibleSize');
    verifyError(testCase,@()setField('optional_value', ["a" "b"]),'MATLAB:validation:IncompatibleSize');
end

function testIntegrationTestObjectCreateAndPopulate(testCase)
    % We can create an IntegrationTestObject object
    integration_test = IntegrationTest;
    integration_test.object_map.foo = DerivedOne;
    integration_test.object_map.bar = DerivedTwo;
    integration_test.object_vec{1} = DerivedOne;
    integration_test.object_vec{2} = DerivedTwo;
    integration_test.object_array{1} = DerivedOne;
    integration_test.object_array{2} = DerivedTwo;
    integration_test.enum_value = Enumeration.value1;
    integration_test.non_poly_derived{1} = NonPolyDerived;
end

function testConvertIntegrationTestObjectToStruct(testCase)
    % We can convert an IntegrationTestObject object to a struct
    integration_test = IntegrationTest;
    integration_test.object_map.foo = DerivedOne;
    integration_test.object_map.bar = DerivedTwo;
    integration_test.object_vec{1} = DerivedOne;
    integration_test.object_vec{2} = DerivedTwo;
    integration_test.object_array{1} = DerivedOne;
    integration_test.object_array{2} = DerivedTwo;
    integration_test.enum_value = Enumeration.value1;
    integration_test.non_poly_derived{1} = NonPolyDerived;
    json_data = integration_test.store_to_struct();
    assert(~isempty(json_data))
end